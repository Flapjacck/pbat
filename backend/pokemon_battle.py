import random
import math
import time
from typing import Dict, List, Tuple, Optional
import requests
import json

class Pokemon:
    def __init__(self, name: str, pokemon_data: dict, ivs: dict, evs: dict, moves: list, level: int = 50):
        self.name = name.capitalize()
        self.pokemon_data = pokemon_data
        self.ivs = ivs
        self.evs = evs
        self.moves = moves
        self.level = level
        self.types = [t['type']['name'] for t in pokemon_data['types']]
        
        # Calculate actual stats
        self.stats = self._calculate_stats()
        self.max_hp = self.stats['hp']
        self.current_hp = self.max_hp
        
        # Status conditions
        self.status = None  # poison, burn, paralysis, sleep, freeze
        self.status_turns = 0
        
        # Battle stats modifications (temporary)
        self.stat_modifiers = {
            'attack': 0,
            'defense': 0,
            'special-attack': 0,
            'special-defense': 0,
            'speed': 0,
            'accuracy': 0,
            'evasion': 0
        }
        
        # Move data cache
        self.move_data = {}
        self._load_move_data()
    
    def _calculate_stats(self) -> dict:
        """Calculate actual stats using Pokemon formula"""
        calculated_stats = {}
        
        for stat_info in self.pokemon_data['stats']:
            stat_name = stat_info['stat']['name']
            base_stat = stat_info['base_stat']
            iv = self.ivs.get(stat_name, 0)
            ev = self.evs.get(stat_name, 0)
            
            if stat_name == 'hp':
                # HP formula
                calculated_stats[stat_name] = math.floor(
                    ((2 * base_stat + iv + math.floor(ev / 4)) * self.level) / 100
                ) + self.level + 10
            else:
                # Other stats formula
                calculated_stats[stat_name] = math.floor(
                    (((2 * base_stat + iv + math.floor(ev / 4)) * self.level) / 100) + 5
                )
        
        return calculated_stats
    
    def _load_move_data(self):
        """Load move data from PokeAPI"""
        print(f"Loading move data for {self.name}...")
        
        for move_name in self.moves:
            try:
                # Try HTTP first, then HTTPS without SSL verification
                urls_to_try = [
                    f"http://pokeapi.co/api/v2/move/{move_name}",
                    f"https://pokeapi.co/api/v2/move/{move_name}"
                ]
                
                move_data = None
                for url in urls_to_try:
                    try:
                        if url.startswith('https'):
                            response = requests.get(url, timeout=10, verify=False, headers={
                                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
                            })
                        else:
                            response = requests.get(url, timeout=10, headers={
                                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
                            })
                        
                        if response.status_code == 200:
                            move_data = response.json()
                            break
                    except Exception:
                        continue
                
                if move_data:
                    self.move_data[move_name] = move_data
                else:
                    # Default move data if API fails
                    self.move_data[move_name] = {
                        'name': move_name,
                        'power': 60,
                        'accuracy': 90,
                        'pp': 20,
                        'type': {'name': 'normal'},
                        'damage_class': {'name': 'physical'},
                        'effect_entries': [{'effect': 'Deals damage with no additional effects.'}]
                    }
                    
            except Exception as e:
                print(f"Warning: Could not load data for move {move_name}: {e}")
                # Use default move data
                self.move_data[move_name] = {
                    'name': move_name,
                    'power': 60,
                    'accuracy': 90,
                    'pp': 20,
                    'type': {'name': 'normal'},
                    'damage_class': {'name': 'physical'},
                    'effect_entries': [{'effect': 'Deals damage with no additional effects.'}]
                }
    
    def get_effective_stat(self, stat_name: str) -> int:
        """Get stat with battle modifications applied"""
        base_stat = self.stats.get(stat_name, 0)
        modifier = self.stat_modifiers.get(stat_name, 0)
        
        # Apply stat modification multiplier
        multiplier = 1.0
        if modifier > 0:
            multiplier = (2 + modifier) / 2
        elif modifier < 0:
            multiplier = 2 / (2 + abs(modifier))
        
        return int(base_stat * multiplier)
    
    def is_fainted(self) -> bool:
        """Check if Pokemon has fainted"""
        return self.current_hp <= 0
    
    def apply_status_damage(self) -> str:
        """Apply damage from status conditions"""
        if not self.status:
            return ""
        
        messages = []
        
        if self.status == 'poison':
            damage = max(1, self.max_hp // 8)
            self.current_hp = max(0, self.current_hp - damage)
            messages.append(f"{self.name} takes {damage} damage from poison!")
            
        elif self.status == 'burn':
            damage = max(1, self.max_hp // 16)
            self.current_hp = max(0, self.current_hp - damage)
            messages.append(f"{self.name} takes {damage} damage from burn!")
        
        # Decrement status turns for temporary statuses
        if self.status in ['sleep', 'freeze']:
            self.status_turns -= 1
            if self.status_turns <= 0:
                messages.append(f"{self.name} woke up!" if self.status == 'sleep' else f"{self.name} thawed out!")
                self.status = None
                self.status_turns = 0
        
        return " ".join(messages)

class BattleEngine:
    def __init__(self):
        self.type_effectiveness = self._load_type_effectiveness()
    
    def _load_type_effectiveness(self) -> dict:
        """Load type effectiveness chart - simplified version"""
        # This is a simplified type chart - in a full implementation you'd load this from PokeAPI
        return {
            'fire': {'grass': 2.0, 'ice': 2.0, 'bug': 2.0, 'steel': 2.0, 'water': 0.5, 'fire': 0.5, 'rock': 0.5, 'dragon': 0.5},
            'water': {'fire': 2.0, 'ground': 2.0, 'rock': 2.0, 'water': 0.5, 'grass': 0.5, 'dragon': 0.5},
            'grass': {'water': 2.0, 'ground': 2.0, 'rock': 2.0, 'fire': 0.5, 'grass': 0.5, 'poison': 0.5, 'flying': 0.5, 'bug': 0.5, 'dragon': 0.5, 'steel': 0.5},
            'electric': {'water': 2.0, 'flying': 2.0, 'electric': 0.5, 'grass': 0.5, 'dragon': 0.5, 'ground': 0.0},
            'psychic': {'fighting': 2.0, 'poison': 2.0, 'psychic': 0.5, 'steel': 0.5, 'dark': 0.0},
            'ice': {'grass': 2.0, 'ground': 2.0, 'flying': 2.0, 'dragon': 2.0, 'fire': 0.5, 'water': 0.5, 'ice': 0.5, 'steel': 0.5},
            'dragon': {'dragon': 2.0, 'steel': 0.5, 'fairy': 0.0},
            'dark': {'psychic': 2.0, 'ghost': 2.0, 'fighting': 0.5, 'dark': 0.5, 'fairy': 0.5},
            'fairy': {'fighting': 2.0, 'dragon': 2.0, 'dark': 2.0, 'fire': 0.5, 'poison': 0.5, 'steel': 0.5},
            'fighting': {'normal': 2.0, 'ice': 2.0, 'rock': 2.0, 'dark': 2.0, 'steel': 2.0, 'poison': 0.5, 'flying': 0.5, 'psychic': 0.5, 'bug': 0.5, 'fairy': 0.5, 'ghost': 0.0},
            'poison': {'grass': 2.0, 'fairy': 2.0, 'poison': 0.5, 'ground': 0.5, 'rock': 0.5, 'ghost': 0.5, 'steel': 0.0},
            'ground': {'fire': 2.0, 'electric': 2.0, 'poison': 2.0, 'rock': 2.0, 'steel': 2.0, 'grass': 0.5, 'bug': 0.5, 'flying': 0.0},
            'flying': {'electric': 0.5, 'ice': 0.5, 'rock': 0.5, 'fighting': 2.0, 'bug': 2.0, 'grass': 2.0, 'steel': 0.5},
            'bug': {'grass': 2.0, 'psychic': 2.0, 'dark': 2.0, 'fire': 0.5, 'fighting': 0.5, 'poison': 0.5, 'flying': 0.5, 'ghost': 0.5, 'steel': 0.5, 'fairy': 0.5},
            'rock': {'fire': 2.0, 'ice': 2.0, 'flying': 2.0, 'bug': 2.0, 'fighting': 0.5, 'ground': 0.5, 'steel': 0.5},
            'ghost': {'psychic': 2.0, 'ghost': 2.0, 'dark': 0.5, 'normal': 0.0},
            'steel': {'ice': 2.0, 'rock': 2.0, 'fairy': 2.0, 'fire': 0.5, 'water': 0.5, 'electric': 0.5, 'steel': 0.5},
            'normal': {'rock': 0.5, 'ghost': 0.0, 'steel': 0.5}
        }
    
    def calculate_type_effectiveness(self, move_type: str, defender_types: List[str]) -> float:
        """Calculate type effectiveness multiplier"""
        effectiveness = 1.0
        
        for defender_type in defender_types:
            type_matchup = self.type_effectiveness.get(move_type, {})
            multiplier = type_matchup.get(defender_type, 1.0)
            effectiveness *= multiplier
        
        return effectiveness
    
    def calculate_damage(self, attacker: Pokemon, defender: Pokemon, move_name: str) -> Tuple[int, str]:
        """Calculate damage dealt by a move"""
        move_data = attacker.move_data.get(move_name)
        if not move_data:
            return 0, "Move data not found!"
        
        # Get move power - some moves (like status moves) don't have power
        power = move_data.get('power')
        if power is None:
            return 0, "Status move used!"
        
        move_type = move_data['type']['name']
        damage_class = move_data['damage_class']['name']
        
        # Determine attack and defense stats to use
        if damage_class == 'physical':
            attack_stat = attacker.get_effective_stat('attack')
            defense_stat = defender.get_effective_stat('defense')
        elif damage_class == 'special':
            attack_stat = attacker.get_effective_stat('special-attack')
            defense_stat = defender.get_effective_stat('special-defense')
        else:
            # Status moves don't deal damage
            return 0, "Status move used!"
        
        # Base damage calculation
        level_factor = (2 * attacker.level / 5 + 2)
        base_damage = (level_factor * power * attack_stat / defense_stat) / 50 + 2
        
        # Apply STAB (Same Type Attack Bonus)
        stab = 1.5 if move_type in attacker.types else 1.0
        
        # Apply type effectiveness
        type_effectiveness = self.calculate_type_effectiveness(move_type, defender.types)
        
        # Apply random factor (85-100%)
        random_factor = random.uniform(0.85, 1.0)
        
        # Calculate final damage
        final_damage = int(base_damage * stab * type_effectiveness * random_factor)
        
        # Create effectiveness message
        effectiveness_msg = ""
        if type_effectiveness > 1:
            effectiveness_msg = "It's super effective!"
        elif type_effectiveness < 1 and type_effectiveness > 0:
            effectiveness_msg = "It's not very effective..."
        elif type_effectiveness == 0:
            effectiveness_msg = "It doesn't affect the opponent!"
        
        return max(1, final_damage), effectiveness_msg
    
    def use_move(self, attacker: Pokemon, defender: Pokemon, move_name: str) -> List[str]:
        """Execute a move and return battle messages"""
        messages = []
        
        # Check if attacker can use the move
        if attacker.is_fainted():
            return [f"{attacker.name} has fainted and cannot move!"]
        
        # Check status conditions that prevent moving
        if attacker.status == 'sleep':
            messages.append(f"{attacker.name} is fast asleep!")
            return messages
        
        if attacker.status == 'freeze':
            messages.append(f"{attacker.name} is frozen solid!")
            return messages
        
        if attacker.status == 'paralysis' and random.random() < 0.25:
            messages.append(f"{attacker.name} is paralyzed and can't move!")
            return messages
        
        move_data = attacker.move_data.get(move_name)
        if not move_data:
            messages.append(f"{attacker.name} doesn't know {move_name}!")
            return messages
        
        # Check accuracy
        accuracy = move_data.get('accuracy')
        if accuracy is None:
            accuracy = 100  # Moves without accuracy always hit (like status moves)
        
        accuracy_modifier = attacker.stat_modifiers.get('accuracy', 0) - defender.stat_modifiers.get('evasion', 0)
        
        # Apply accuracy modification
        final_accuracy = accuracy
        if accuracy_modifier != 0:
            if accuracy_modifier > 0:
                final_accuracy = accuracy * (3 + accuracy_modifier) / 3
            else:
                final_accuracy = accuracy * 3 / (3 + abs(accuracy_modifier))
        
        messages.append(f"{attacker.name} used {move_name.replace('-', ' ').title()}!")
        
        if random.randint(1, 100) > final_accuracy:
            messages.append(f"{attacker.name}'s attack missed!")
            return messages
        
        # Calculate and apply damage
        damage, effectiveness_msg = self.calculate_damage(attacker, defender, move_name)
        
        if damage > 0:
            defender.current_hp = max(0, defender.current_hp - damage)
            messages.append(f"{defender.name} takes {damage} damage!")
            
            if effectiveness_msg:
                messages.append(effectiveness_msg)
            
            # Check for critical hit (simplified - 6.25% chance)
            if random.random() < 0.0625:
                messages.append("Critical hit!")
        
        # Apply status effects (simplified)
        self._apply_move_effects(attacker, defender, move_name, messages)
        
        return messages
    
    def _apply_move_effects(self, attacker: Pokemon, defender: Pokemon, move_name: str, messages: List[str]):
        """Apply special effects of moves (simplified implementation)"""
        # This is a very simplified version - a full implementation would parse move effects from PokeAPI
        move_name_lower = move_name.lower()
        
        # Some basic status move effects
        if 'poison' in move_name_lower and not defender.status and random.random() < 0.3:
            defender.status = 'poison'
            messages.append(f"{defender.name} was poisoned!")
        
        elif 'burn' in move_name_lower or 'flame' in move_name_lower and not defender.status and random.random() < 0.1:
            defender.status = 'burn'
            messages.append(f"{defender.name} was burned!")
        
        elif 'paralyze' in move_name_lower or 'thunder' in move_name_lower and not defender.status and random.random() < 0.1:
            defender.status = 'paralysis'
            messages.append(f"{defender.name} was paralyzed!")
        
        elif 'sleep' in move_name_lower and not defender.status and random.random() < 0.3:
            defender.status = 'sleep'
            defender.status_turns = random.randint(1, 3)
            messages.append(f"{defender.name} fell asleep!")
    
    def battle_turn(self, pokemon1: Pokemon, pokemon2: Pokemon, move1: str, move2: str) -> List[str]:
        """Execute one turn of battle"""
        messages = []
        
        # Determine turn order based on speed
        speed1 = pokemon1.get_effective_stat('speed')
        speed2 = pokemon2.get_effective_stat('speed')
        
        if speed1 > speed2 or (speed1 == speed2 and random.choice([True, False])):
            first, second = pokemon1, pokemon2
            first_move, second_move = move1, move2
        else:
            first, second = pokemon2, pokemon1
            first_move, second_move = move2, move1
        
        # First Pokemon attacks
        messages.extend(self.use_move(first, second, first_move))
        
        # Second Pokemon attacks (if not fainted)
        if not second.is_fainted():
            messages.extend(self.use_move(second, first, second_move))
        
        # Apply status damage
        for pokemon in [pokemon1, pokemon2]:
            if not pokemon.is_fainted():
                status_msg = pokemon.apply_status_damage()
                if status_msg:
                    messages.append(status_msg)
        
        return messages

class BattleSimulator:
    def __init__(self):
        self.battle_engine = BattleEngine()
    
    def simulate_battle(self, pokemon1: Pokemon, pokemon2: Pokemon, auto_battle: bool = False) -> str:
        """Simulate a complete battle between two Pokemon"""
        print(f"\n{'='*60}")
        print(f"POKEMON BATTLE: {pokemon1.name} vs {pokemon2.name}")
        print(f"{'='*60}")
        
        # Display Pokemon info
        self._display_battle_info(pokemon1, pokemon2)
        
        turn_count = 0
        max_turns = 100  # Prevent infinite battles
        
        while not pokemon1.is_fainted() and not pokemon2.is_fainted() and turn_count < max_turns:
            turn_count += 1
            print(f"\n--- Turn {turn_count} ---")
            
            # Get moves for this turn
            if auto_battle:
                move1 = random.choice(pokemon1.moves)
                move2 = random.choice(pokemon2.moves)
                print(f"{pokemon1.name} will use {move1.replace('-', ' ').title()}")
                print(f"{pokemon2.name} will use {move2.replace('-', ' ').title()}")
            else:
                move1 = self._get_player_move(pokemon1, 1)
                move2 = self._get_player_move(pokemon2, 2)
            
            # Execute turn
            turn_messages = self.battle_engine.battle_turn(pokemon1, pokemon2, move1, move2)
            
            # Display turn results
            for message in turn_messages:
                print(message)
                if not auto_battle:
                    time.sleep(0.5)  # Dramatic pause for manual battles
            
            # Display current HP
            print(f"\n{pokemon1.name}: {pokemon1.current_hp}/{pokemon1.max_hp} HP")
            print(f"{pokemon2.name}: {pokemon2.current_hp}/{pokemon2.max_hp} HP")
            
            if auto_battle and turn_count % 3 == 0:
                time.sleep(1)  # Pause every few turns in auto battle
        
        # Determine winner
        if pokemon1.is_fainted() and pokemon2.is_fainted():
            winner = "It's a tie!"
        elif pokemon1.is_fainted():
            winner = f"{pokemon2.name} wins!"
        elif pokemon2.is_fainted():
            winner = f"{pokemon1.name} wins!"
        else:
            winner = "Battle reached maximum turns - it's a draw!"
        
        print(f"\n{'='*60}")
        print(f"BATTLE RESULT: {winner}")
        print(f"Battle lasted {turn_count} turns")
        print(f"{'='*60}")
        
        return winner
    
    def _display_battle_info(self, pokemon1: Pokemon, pokemon2: Pokemon):
        """Display initial battle information"""
        for i, pokemon in enumerate([pokemon1, pokemon2], 1):
            print(f"\nPokemon {i}: {pokemon.name}")
            print(f"  Level: {pokemon.level}")
            print(f"  Types: {', '.join(pokemon.types).title()}")
            print(f"  HP: {pokemon.max_hp}")
            print(f"  Attack: {pokemon.stats['attack']}")
            print(f"  Defense: {pokemon.stats['defense']}")
            print(f"  Sp. Atk: {pokemon.stats['special-attack']}")
            print(f"  Sp. Def: {pokemon.stats['special-defense']}")
            print(f"  Speed: {pokemon.stats['speed']}")
            print(f"  Moves: {', '.join([move.replace('-', ' ').title() for move in pokemon.moves])}")
    
    def _get_player_move(self, pokemon: Pokemon, player_num: int) -> str:
        """Get move choice from player"""
        print(f"\nPlayer {player_num} ({pokemon.name}), choose your move:")
        for i, move in enumerate(pokemon.moves, 1):
            move_data = pokemon.move_data.get(move, {})
            power = move_data.get('power', 'N/A')
            move_type = move_data.get('type', {}).get('name', 'unknown')
            print(f"  {i}. {move.replace('-', ' ').title()} (Type: {move_type.title()}, Power: {power})")
        
        while True:
            try:
                choice = input("Enter move number (1-4): ").strip()
                move_index = int(choice) - 1
                if 0 <= move_index < len(pokemon.moves):
                    return pokemon.moves[move_index]
                else:
                    print("Invalid choice. Please enter 1-4.")
            except ValueError:
                print("Please enter a valid number.")
